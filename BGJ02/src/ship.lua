require("util/helper")
require("util/vector")
require("polygonentity")
require("bullet")

Ship = class("Ship", PolygonEntity)

function Ship:__init()
    PolygonEntity.__init(self)
    --self.velocity = Vector(20, 10)

    self:addPoint(Vector(10, 0))
    self:addPoint(Vector(-10, -7))
    self:addPoint(Vector(-3, 0))
    self:addPoint(Vector(-10, 7))

    self.timeUntilShoot = 0

    turn_speed = math.pi
end

function Ship:update(dt)
    PolygonEntity.update(self, dt)

    local bouncy = 0.8
    max_x = 300
    max_y = 200
    if self.position.x > max_x then
        self.position.x = max_x
        self.velocity.x = -self.velocity.x * bouncy
    end
    if self.position.x <-max_x then
        self.position.x =-max_x
        self.velocity.x = -self.velocity.x * bouncy
    end
    if self.position.y > max_y then
        self.position.y = max_y
        self.velocity.y = -self.velocity.y * bouncy
    end
    if self.position.y <-max_y then
        self.position.y =-max_y
        self.velocity.y = -self.velocity.y * bouncy
    end

    if self.timeUntilShoot > 0 then
        self.timeUntilShoot = self.timeUntilShoot - dt
    end
end

function Ship:draw()
    love.graphics.setColor(255, 255, 255)
    love.graphics.polygon("line", self:getDrawPoints())
end

function Ship:move(forward, dt)
    local f = math.max(0, math.min(1, forward))

    self.velocity = self.velocity + Vector(f * 500 * dt, 0):rotated(self.rotation)
    max = 200
    if self.velocity:len() > max then
        self.velocity:normalize()
        self.velocity = self.velocity * max
    end
end

function Ship:shoot()
    if self.timeUntilShoot <= 0 then
        local b = Bullet(self.position, self.velocity, self.rotation)
        self.world:add(b)
        self.timeUntilShoot = 0.5
    end
end
